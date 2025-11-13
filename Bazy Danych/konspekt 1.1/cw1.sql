-- zadanie 1
select p.UnitPrice,p.ProductName  from Products p where p.CategoryID = 6

-- zadanie 2
select p.ProductName , p.UnitsInStock  from Products p where p.SupplierID  = 4

-- zadanie 3
select p.ProductName from Products p where p.UnitsInStock = 0