-- zadanie 1
select o.ProductID , o.Quantity * (o.UnitPrice * (1 - o.Discount )) as 'cena' from [Order Details] o where o.OrderID  = 10250

-- zadanie 2
select isnull(s.Phone,'')+','+isnull(s.Fax ,'') from Suppliers s